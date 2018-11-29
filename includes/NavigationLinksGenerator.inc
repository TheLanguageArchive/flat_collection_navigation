<?php
/**
 * @author  Ibrahim Abdullah
 * @package Flat Collection Navigation
 */
class NavigationLinksGenerator
{
    /**
     * @var AbstractObject
     */
    private $object;

    /**
     * Constructor
     *
     * @param AbstractObject $object
     */
    public function __construct(AbstractObject $object)
    {
        $this->object = $object;
    }

    /**
     * Get sibling pids
     *
     * @return array
     */
    public function getSiblings(): array
    {
        $siblings = [];
        $objects  = islandora_basic_collection_get_member_objects($this->object, 0, 20, 'view', 'islandora:sp_cmdiCModel');

        if (is_array($objects) && isset($objects[1])) {

            foreach ($objects[1] as $object) {
                $siblings[] = $object['object']['value'];
            }
        }

        return $siblings;
    }

    /**
     * Generating navigation links
     *
     * @return array|false
     */
    public function generate($current)
    {
        $siblings = $this->getSiblings();
        $position = array_search($current, $siblings);
        $previous = false;
        $next     = false;

        if (count($siblings) < 2 || false === $position) {
            return false;
        }

        while (key($siblings) !== $position) {
            next($siblings);
        }

        if ($position > 0) {

            $previous = prev($siblings);
            next($siblings);
        }

        $next  = next($siblings);
        $links = [];

        return [
            'previous' => $this->generateLink('Previous', $previous),
            'next'     => $this->generateLink('Next', $next),
        ];
    }

    /**
     * Generate link to islandora object
     *
     * @param string       $label
     * @param string|false $pid
     *
     * @return string
     */
    public function generateLink(string $label, $pid = false): string
    {
        return (false === $pid ? $label : l($label, 'islandora/object/' . $pid));
    }

    /**
     * Parents are saved in an array in $variables
     * determine closest parent and return new instance
     *
     * @param array $parents
     *
     * @return NavigationLinks
     * @throws \Exception
     */
    public static function create(array &$parents): self
    {
        if (count($parents) === 0) {
            throw new \Exception('Could not find parent');
        }

        return new self($parents[0]);
    }
}
